# 2023학년도 캡스톤 디자인<지도형> : 모바일 렌더링 소프트웨어 개선
### Capstone-Design(gles3jni)
## 팀정보
|팀원|직급|
|:--:|:--:|
|김지민|팀장|
|김대경|팀원|
|문정호|팀원|
|임지호|팀원|
|조대훈|팀원|

## 개요
모바일 환경은 데스크톱 PC에 비해 여러 가지 제약이 존재한다. 하지만 모바일 하드웨어/소프트웨어의 발달로 데스크톱에서 쓰이는 여러 가지 렌더링 기술이 모바일로 많이 이식되어가고 있다. 그래픽스 효과에는 여러 가지가 있지만, 그중에서도 모바일에서는 다루기 힘들었던 many-light 렌더링을 모바일 환경에서 구현하는 것을 목표로 설정했다. 안드로이드 스튜디오 환경에서 JNI를 사용하여 Java와 C++의 통합 코드로써 프로젝트를 진행했다. Java 코드를 통해 안드로이드 앱 환경을 구축하고, C++ 코드를 통한 렌더링을 실행한다.
Assimp 라이브러리를 활용하여 Sponza를 로드하고, Tiled Forward Shading 기법을 적용하는 방식으로 프로젝트를 진행했다.


## 구현결과
Tiled Forward Shading은 실시간 그래픽스에서 사용되는 렌더링 기술중 하나로, 조명 및 셰이딩 연산을 빠르게 수행하기 위해 화면을 작은 타일로 나누어 그 타일에 대해서만 연산을 진행하는 방식이다. 그 결과로 다수의 동적 라이트 소스와 더 복잡한 머티리얼 및 셰이딩을 처리하는데 효과적이다.
그 결과, Unity의 real-time lights 제한을 훨씬 넘긴 real-time lights를 구현할 수 있었으며, 라이트 소스를 높이면 프레임이 급격히 떨어지는 Unity에 비해  Tiled Forward Shading을 구현한 씬에서는 준수한 프레임을 보여주었다.

## 스크린샷
|화면 명| 화면 |
|:--:|:--:|
|초기 화면 : light == 0|<img src="https://github.com/2023-SMU-Capstone-design/gles3jni/assets/87518434/3b344e18-2a58-4cca-a3db-d1ed61e3485c"></img>|
|다수 light 적용 화면|<img src="https://github.com/2023-SMU-Capstone-design/gles3jni/assets/87518434/b0153a1b-81c8-40b6-bee7-a7e7e89f5509"></img>|
|Unity 비교|<img src="https://github.com/2023-SMU-Capstone-design/gles3jni/assets/87518434/a5db96bd-eb39-40b4-802b-9205975f1914"></img>|




