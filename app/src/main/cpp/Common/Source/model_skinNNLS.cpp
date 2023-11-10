//SkinNNLS file loader
//Jae-Ho Nah, 2014/07

//The detailed description of the NNLS format is described in
//http://graphics.cs.cmu.edu/projects/sma/textData/



#include <Common/Include/model_skinNNLS.h>

#include <stdio.h>

cNNLS::cNNLS()
{
    nFrames=0;
    nBones=0;
    nVertices=0;
    nDisplacementCorrections=3; //maximum value: 5 (due to limited vertex attribute numbers)
}


cNNLS::~cNNLS()
{
    destroy();
}

void cNNLS::destroy()
{

    for (int i=0; i<MAX_FRAMES; i++)
    {
        boneAnimation[i].clear();
        displacementCoords[i].clear();
        normalCoords[i].clear();
        boneAnimation[i].shrink_to_fit();
        displacementCoords[i].shrink_to_fit();
        normalCoords[i].shrink_to_fit();

    }
    vertexWeightsIndices.clear();
    vertexWeightsValues.clear();

    vertexWeightsIndices.shrink_to_fit();
    vertexWeightsValues.shrink_to_fit();

    for (int i=0; i<3; i++)
    {
        displacementBasis[i].clear();
        normalBasis[i].clear();

        displacementBasis[i].shrink_to_fit();
        normalBasis[i].shrink_to_fit();
    }

    nFrames=0;
    nBones=0;
    nVertices=0;
    nDisplacementCorrections=3;
}


bool cNNLS::loadFile(const char* filename, int userDisplacementCorrections)
{
    FILE *pFile = fopen(filename, "r");
    nDisplacementCorrections = (userDisplacementCorrections>3)?3:userDisplacementCorrections;

    if (!pFile)
        return false;

    //initialize
    char buffer[1024] = {0};
    this->nBones = 0;
    this->nFrames = 0;
    this->nVertices = 0;


    //while (fscanf(pFile, "%s", buffer) != EOF)
    while (fgets(buffer, sizeof(buffer), pFile) != NULL)
    {
        switch (buffer[0])
        {
        case '*':
            if (strstr(buffer, "OBJFILENAME"))
            {
                fgets(buffer, sizeof(buffer), pFile);
                sprintf (this->objFilename, "%s", buffer);
                for (int n=0;n<30;n++)
                {
                    if (this->objFilename[n] ==10)
                    {
                        this->objFilename[n] =0;//delete line feed
                        break;
                    }
                }
            }

            else if (strstr(buffer, "BONEANIMATION"))
            {
                int boneIndex, nFrames;
                sscanf (buffer, "*BONEANIMATION, BONEINDEX=%d, NFRAMES=%d", &boneIndex, &nFrames);
                this->nBones = boneIndex+1;
                this->nFrames = nFrames;

                for (int frame=0; frame<this->nFrames; frame++)
                {
                    int temp;
                    fscanf (pFile, "%d", &temp); //current frame, but pass

                    ESMatrix mat;
                    for (int m=0; m<4; m++)
                        for (int n=0; n<4; n++)
                            fscanf(pFile, "%f", &mat.m[n][m]);//must transpose? row major order --> column major order

                    this->boneAnimation[frame].push_back(mat);
                }
            }
            else if (strstr(buffer, "VERTEXWEIGHTS"))
            {
                sscanf (buffer, "*VERTEXWEIGHTS, NVERTICES=%d", &this->nVertices);

                for (int vertex=0; vertex<this->nVertices; vertex++)
                {
                    int temp;
                    fscanf (pFile, "%d", &temp); //current vertex, but pass

                    glm::uvec4 vertexWeightIndex;
                    glm::vec4 vertexWeightValue;

                    for (int n=0; n<4; n++)
                        fscanf(pFile, "%d %f",  &vertexWeightIndex[n], &vertexWeightValue[n]);


                    this->vertexWeightsIndices.push_back(vertexWeightIndex);
                    this->vertexWeightsValues.push_back(vertexWeightValue);
                }
            }

            else if (strstr(buffer, "DISPLACEMENTBASIS"))
            {
                for (int vertex=0; vertex<this->nVertices; vertex++)
                {
                    float displacementBasisValue[5][3]={0};
                    for (int axis=0; axis<3; axis++)
                    {
                        //std::vector<float> displacementBasisScalar;
                        if (fgets(buffer, sizeof(buffer), pFile)==NULL)
                        {
                            //error when loading a displamcent basis
                            return false;
                        }

                        //support 0~3 corrections

                        switch (this->nDisplacementCorrections)
                        {
                        case 1:
                            sscanf(buffer, "%f",  &displacementBasisValue[0][axis]);
                            break;
                        case 2:
                            sscanf(buffer, "%f %f",  &displacementBasisValue[0][axis],&displacementBasisValue[1][axis]);
                            break;
                        case 3:
                            sscanf(buffer, "%f %f %f",  &displacementBasisValue[0][axis],&displacementBasisValue[1][axis],&displacementBasisValue[2][axis]);
                            break;
                            /*
                        case 4:
                            sscanf(buffer, "%f %f %f %f",  &displacementBasisValue[0],&displacementBasisValue[1],&displacementBasisValue[2],&displacementBasisValue[3]);
                            break;
                        case 5:
                            sscanf(buffer, "%f %f %f %f %f",  &displacementBasisValue[0],&displacementBasisValue[1],&displacementBasisValue[2],&displacementBasisValue[3],&displacementBasisValue[4]);
                            break;
                            */
                        }

                        /*
                        for (int n=0; n<this->nDisplacementCorrections; n++)
                        {
                            this->displacementBasis.push_back(displacementBasisValue[n]);
                        }
                        */
                    }


                    for (int n=0; n<this->nDisplacementCorrections; n++)
                    {
                        for (int axis=0; axis<3; axis++)
                        {
                            this->displacementBasis[axis].push_back(displacementBasisValue[n][axis]);
                        }
                    }


                }
            }

            else if (strstr(buffer, "DISPLACEMENTCOORDS"))
            {
                //float displacementCoord;
                for (int frame=0; frame<this->nFrames; frame++)
                {
                    if (fgets(buffer, sizeof(buffer), pFile)==NULL)
                    {
                        //error when loading a displamcent coord
                        return false;
                    }

                    //support 0~3 corrections
                    float displacementCoordValue[5]={0};
                    switch (this->nDisplacementCorrections)
                    {
                    case 1:
                        sscanf(buffer, "%f",  &displacementCoordValue[0]);
                        break;
                    case 2:
                        sscanf(buffer, "%f %f",  &displacementCoordValue[0],&displacementCoordValue[1]);
                        break;
                    case 3:
                        sscanf(buffer, "%f %f %f",  &displacementCoordValue[0],&displacementCoordValue[1],&displacementCoordValue[2]);
                        break;
                        /*
                    case 4:
                        sscanf(buffer, "%f %f %f %f",  &displacementCoordValue[0],displacementCoordValue[1],displacementCoordValue[2],displacementCoordValue[3]);
                        break;
                    case 5:
                        sscanf(buffer, "%f %f %f %f %f",  &displacementCoordValue[0],displacementCoordValue[1],displacementCoordValue[2],displacementCoordValue[3],displacementCoordValue[4]);
                        break;
                        */
                    }

                    for (int n=0; n<this->nDisplacementCorrections; n++)
                    {
                        this->displacementCoords[frame].push_back(displacementCoordValue[n]);
                    }
                }

            }

            else if (strstr(buffer, "NORMALBASIS"))
            {
                char tempchar2[256];
                sprintf(tempchar2, "%d", this->nVertices);

                for (int vertex=0; vertex<this->nVertices; vertex++)
                {
                    //support 0~3 corrections
                    float normalBasisValue[5][3]={0};

                    for (int axis=0; axis<3; axis++)
                    {
                        sprintf(tempchar2, "%d", vertex);
                        if (fgets(buffer, sizeof(buffer), pFile)==NULL)
                        {
                            //error when loading a normal basis
                            return false;
                        }


                        switch (this->nDisplacementCorrections)
                        {
                        case 1:
                            sscanf(buffer, "%f",  &normalBasisValue[0][axis]);
                            break;
                        case 2:
                            sscanf(buffer, "%f %f", &normalBasisValue[0][axis], &normalBasisValue[1][axis]);
                            break;
                        case 3:
                            sscanf(buffer, "%f %f %f", &normalBasisValue[0][axis], &normalBasisValue[1][axis], &normalBasisValue[2][axis]);
                            break;
                            /*
                        case 4:
                            sscanf(buffer, "%f %f %f %f",  &normalBasisValue[0],&normalBasisValue[1],&normalBasisValue[2],&normalBasisValue[3]);
                            break;
                        case 5:
                            sscanf(buffer, "%f %f %f %f %f",  &normalBasisValue[0],&normalBasisValue[1],&normalBasisValue[2],&normalBasisValue[3],&normalBasisValue[4]);
                            break;
                            */
                        }

                        /*
                        for (int n=0; n<this->nDisplacementCorrections; n++)
                        {
                            this->normalBasis.push_back(normalBasisValue[n]);
                        }*/
                    }
                    for (int n=0; n<this->nDisplacementCorrections; n++)
                    {
                        for (int axis=0; axis<3; axis++)
                        {
                            this->normalBasis[axis].push_back(normalBasisValue[n][axis]);
                        }
                    }

                }
            }

            else if (strstr(buffer, "NORMALCOORDS"))
            {
                for (int frame=0; frame<this->nFrames; frame++)
                {
                    char tempchar[256];
                    sprintf(tempchar, "%d %d", (int)frame, this->nFrames);

                    //support 0~3 corrections
                    float normalCoordValue[5]={0};

                    if (fgets(buffer, sizeof(buffer), pFile)==NULL)
                    {
                        //error when loading a normal coord
                        return false;
                    }

                    switch (this->nDisplacementCorrections)
                    {
                    case 1:
                        sscanf(buffer, "%f",  &normalCoordValue[0]);
                        break;
                    case 2:
                        sscanf(buffer, "%f %f",  &normalCoordValue[0],&normalCoordValue[1]);
                        break;
                    case 3:
                        sscanf(buffer, "%f %f %f",  &normalCoordValue[0],&normalCoordValue[1],&normalCoordValue[2]);
                        break;
                        /*
                    case 4:
                        sscanf(buffer, "%f %f %f %f",  &normalCoordValue[0],normalCoordValue[1],normalCoordValue[2],normalCoordValue[3]);
                        break;
                    case 5:
                        sscanf(buffer, "%f %f %f %f %f",  &normalCoordValue[0],normalCoordValue[1],normalCoordValue[2],normalCoordValue[3],normalCoordValue[4]);
                        break;
                        */
                    }

                    for (int n=0; n<this->nDisplacementCorrections; n++)
                    {
                        this->normalCoords[frame].push_back(normalCoordValue[n]);
                        //char tempchar[256];
                        sprintf(tempchar, "%d %d %f", (int)frame, this->normalCoords[(int)frame].size(), normalCoordValue[n]);
                    }
                }
            }

            break;

        case '#':    //skip an annotation
            break;

        default:
            fgets(buffer, sizeof(buffer), pFile); //skip the line
            break;
        }


    }

    return true;
}



#if BINARY_INPUT_OUTPUT
void cNNLS::serialize(FILE* fp)
{
	write_binary_single(fp, nFrames);
	write_binary_single(fp, nBones);
	write_binary_single(fp, nVertices);
	write_binary_single(fp, nDisplacementCorrections);

	for (int i = 0; i < nFrames; i++)
		write_binary_vector(fp, boneAnimation[i], sizeof(ESMatrix), boneAnimation[i].size());

	write_binary_vector(fp, vertexWeightsIndices, sizeof(glm::uvec4), vertexWeightsIndices.size());
	write_binary_vector(fp, vertexWeightsValues, sizeof(glm::vec4), vertexWeightsValues.size());

	for (int i = 0; i < 3; i++)
		write_binary_vector(fp, displacementBasis[i], sizeof(float), displacementBasis[i].size());

	for (int i = 0; i < nFrames; i++)
		write_binary_vector(fp, displacementCoords[i], sizeof(float), displacementCoords[i].size());

	for (int i = 0; i < 3; i++)
		write_binary_vector(fp, normalBasis[i], sizeof(float), normalBasis[i].size());

	for (int i = 0; i < nFrames; i++)
		write_binary_vector(fp, normalCoords[i], sizeof(float), normalCoords[i].size());

	write_binary_array(fp, objFilename, sizeof(char), 30);
}

void cNNLS::deserialize(FILE* fp)
{
	read_binary_single(fp, nFrames);
	read_binary_single(fp, nBones);
	read_binary_single(fp, nVertices);
	read_binary_single(fp, nDisplacementCorrections);

	for (int i = 0; i < nFrames; i++)
		read_binary_vector(fp, boneAnimation[i], sizeof(ESMatrix));

	read_binary_vector(fp, vertexWeightsIndices, sizeof(glm::uvec4));
	read_binary_vector(fp, vertexWeightsValues, sizeof(glm::vec4));

	for (int i = 0; i < 3; i++)
		read_binary_vector(fp, displacementBasis[i], sizeof(float));

	for (int i = 0; i < nFrames; i++)
		read_binary_vector(fp, displacementCoords[i], sizeof(float));

	for (int i = 0; i < 3; i++)
		read_binary_vector(fp, normalBasis[i], sizeof(float));

	for (int i = 0; i < nFrames; i++)
		read_binary_vector(fp, normalCoords[i], sizeof(float));

	read_binary_array(fp, objFilename, sizeof(char));

#if 0
	char temp_char[256];
	read_binary_array(fp, temp_char, sizeof(unsigned char));
	m_directoryPath = temp_char;

	int temp_bool;
#endif
}



template<typename T>
void cNNLS::write_binary_single(FILE *fp, T& single_reference)
{
	fwrite(&single_reference, sizeof(T), 1, fp);
	fflush(fp);
}

template<typename T>
void cNNLS::write_binary_array(FILE *fp, T& array_reference, int size, int length)
{
	fwrite(&length, sizeof(int), 1, fp);
	if (length>0) fwrite(&array_reference, size, length, fp);
	fflush(fp);
}

template<typename T>
void cNNLS::write_binary_vector(FILE *fp, T& vector_reference, int size, int length)
{
	fwrite(&length, sizeof(int), 1, fp);
	for (int i = 0; i < length; i++)
	{
		fwrite(&vector_reference[i], size, 1, fp);
		fflush(fp);
	}
	fflush(fp);
}
template<typename T>
void cNNLS::read_binary_single(FILE *fp, T& single_reference)
{
	fread(&single_reference, sizeof(T), 1, fp);
	//std::cout << single_value << std::endl;
}

template<typename T>
void cNNLS::read_binary_array(FILE *fp, T& array_reference, int size)
{
	int length;
	fread(&length, sizeof(int), 1, fp);
	if (length>0) fread(array_reference, size, length, fp);
	//std::cout << length << " " << array_value << std::endl;
}

template<typename T>
void cNNLS::read_binary_vector(FILE *fp, std::vector<T>& vector_reference, int size)
{
	int length;
	fread(&length, sizeof(int), 1, fp);

	T* temp = new T[length];
	fread(&temp[0], size, length, fp);
	vector_reference.resize(length);
	memcpy(&vector_reference[0], temp, length*size);
	delete[] temp;
}
#endif
