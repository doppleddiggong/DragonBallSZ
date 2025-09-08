// Fill out your copyright notice in the Description page of Project Settings.


#include "ADynamicCameraActor.h"

#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"


// Sets default values
ADynamicCameraActor::ADynamicCameraActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Spring Arm"));
	SetRootComponent(SpringArm);

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm);
}

// Called when the game starts or when spawned
void ADynamicCameraActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADynamicCameraActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

