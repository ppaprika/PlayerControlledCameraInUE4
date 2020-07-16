// Fill out your copyright notice in the Description page of Project Settings.


#include "PawnWithCamera.h"
#include "Components/SceneComponent.h"


// Sets default values
APawnWithCamera::APawnWithCamera()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	OurCameraSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("OurCameraSpringArm"));
	OurCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("OurCamera"));
	PlayerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlayerMesh"));
	PlayerMesh->SetupAttachment(OurCameraSpringArm);

	
	

	OurCameraSpringArm->SetupAttachment(RootComponent);
	OurCameraSpringArm->SetRelativeLocationAndRotation(FVector(0.0f, 0.0f, 50.0f), FRotator(-60.0f, 0.0f, 0.0f));
	OurCameraSpringArm->TargetArmLength = 400.0f;
	OurCameraSpringArm->bEnableCameraLag = true;
	OurCameraSpringArm->CameraRotationLagSpeed = 3.0f;

	OurCamera->SetupAttachment(OurCameraSpringArm, USpringArmComponent::SocketName);

	AutoPossessPlayer = EAutoReceiveInput::Player0;
}

// Called when the game starts or when spawned
void APawnWithCamera::BeginPlay()
{
	Super::BeginPlay();
	
}

void APawnWithCamera::MoveForward(float Value)
{
	MovementInput.X = FMath::Clamp<float>(Value, -1.0f, 1.0f);
}

void APawnWithCamera::MoveRight(float Value)
{
	MovementInput.Y = FMath::Clamp<float>(Value, -1.0f, 1.0f);
}

void APawnWithCamera::PitchCamera(float Value)
{
	CameraInput.Y = FMath::Clamp<float>(Value, -1.0f, 1.0f);
}

void APawnWithCamera::YawCamera(float Value)
{
	CameraInput.X = FMath::Clamp<float>(Value, -1.0f, 1.0f);
}

void APawnWithCamera::ZoomIn()
{
	bZoomingIn = true;
}

void APawnWithCamera::ZoomOut()
{
	bZoomingIn = false;
}

// Called every frame
void APawnWithCamera::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bZoomingIn) {
		ZoomFactor += DeltaTime / 0.5f;
	}
	else {
		ZoomFactor -= DeltaTime / 0.25f;
	}
	ZoomFactor = FMath::Clamp<float>(ZoomFactor, 0.0f, 1.0f);
	OurCamera->FieldOfView = FMath::Lerp<float>(90.0f, 60.0f, ZoomFactor);
	OurCameraSpringArm->TargetArmLength = FMath::Lerp<float>(400.0f, 300.0f, ZoomFactor);

	//处理物体随鼠标旋转
	{
		FRotator NewRotator = GetActorRotation();
		NewRotator.Yaw += CameraInput.X * Sensitivity;
		SetActorRotation(NewRotator);
	}
	//处理镜头随鼠标俯仰
	{
		FRotator NewRotator = OurCameraSpringArm->GetComponentRotation();
		NewRotator.Pitch = FMath::Clamp<float>(NewRotator.Pitch + CameraInput.Y * Sensitivity, -80.0f, -15.0f);
		OurCameraSpringArm->SetWorldRotation(NewRotator);
	}
	//处理移动
	{
		if (!MovementInput.IsZero()) {
			MovementInput = MovementInput.GetSafeNormal() * 100.0f;
			FVector NewLocation = GetActorLocation();
			NewLocation += GetActorForwardVector() * MovementInput.X * DeltaTime;
			NewLocation += GetActorRightVector() * MovementInput.Y * DeltaTime;
			SetActorLocation(NewLocation);
		}
	}
	//处理跳跃
	if (bJumping) {
		if(JumpFactor >= 0.5f)
			JumpFactor -= DeltaTime / 0.2f;
		JumpFactor = FMath::Clamp<float>(JumpFactor, 0.5f, 1.0f);
		PlayerMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, JumpFactor));
	}
	else {
		JumpFactor = 1.0f;
		PlayerMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
	}
}

// Called to bind functionality to input
void APawnWithCamera::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &APawnWithCamera::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APawnWithCamera::MoveRight);
	PlayerInputComponent->BindAxis("CameraPitch", this, &APawnWithCamera::PitchCamera);
	PlayerInputComponent->BindAxis("CameraYaw", this, &APawnWithCamera::YawCamera);

	PlayerInputComponent->BindAction("ZoomIn", IE_Pressed, this, &APawnWithCamera::ZoomIn);
	PlayerInputComponent->BindAction("ZoomIn", IE_Released, this, &APawnWithCamera::ZoomOut);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &APawnWithCamera::JumpPressed);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &APawnWithCamera::JumpRealeased);
}

