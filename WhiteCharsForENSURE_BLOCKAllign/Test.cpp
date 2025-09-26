// Copyright Epic Games, Inc. All Rights Reserved.

#include "u560fpsCharacter.h"

#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/LocalPlayer.h"

#include <iostream>

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "InputActionValue.h"

#define ENSURE_BLOCK(expression)                                                                                       \
	if (expression)                                                                                                    \
	{                                                                                                                  \
		std::cout << "Error: " << #expression << std::endl;                                                            \
	}                                                                                                                  \
	else

#define ENSURE_BLOCK_DOUBLE(condition1, condition2) \
	bool bGo = true;\
	if(condition1){\
	std::cout << "Error: " << #condition1 << std::endl;\
	bGo = false;\
	}\
	if(condition2){\
	std::cout << "Error: " << #condition2 << std::endl;\
	bGo = false;\
	}\
	if(!bGo) std::cout<<"One of conditions is false"<<std::endl; else


#define ANOTHER_MACRO {std::cout<<"EW";}
#define THIRD_MACRO {std::cout<<"EW";}

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

Au560fpsCharacter::Au560fpsCharacter()
{

	ENSURE_BLOCK_DOUBLE(false, 5 > 10)
	ENSURE_BLOCK(false)
	{

	}

	ENSURE_BLOCK(false)
		ENSURE_BLOCK(5 > 10)
			ENSURE_BLOCK(this == nullptr)
			{
				std::cout << "qwe";
				if (1==2)
				{
					std::cout << "RE}sRW";
				}
			}

	ENSURE_BLOCK(false)
		ANOTHER_MACRO

	ANOTHER_MACRO
	ANOTHER_MACRO

	ANOTHER_MACRO
	THIRD_MACRO

	ENSURE_BLOCK(false)
	{
		std::cout << "qwe";
	}

	if (true)
	{
	}
	else if (5 < 10)
	{
	}
	else if (this != nullptr)
	{
	}
	else
		std::cout << "QQ";
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

	// Note: For faster iteration times these variables, and many more, can be
	// tweaked in the Character Blueprint instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 500.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f;
	CameraBoom->bUsePawnControlRotation = true;

	selfPtr = this;

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// Note: The skeletal mesh and anim blueprint references on the Mesh component
	// (inherited from Character) are set in the derived blueprint asset named
	// ThirdPersonCharacter (to avoid direct content references in C++)
}

void Au560fpsCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &Au560fpsCharacter::Move);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &Au560fpsCharacter::Look);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &Au560fpsCharacter::Look);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error,
		       TEXT("'%s' Failed to find an Enhanced Input component! This template "
			       "is built to use the Enhanced Input system. If you intend to use "
			       "the legacy system, then you will need to update this C++ file."),
		       *GetNameSafe(this));
	}
}

void Au560fpsCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	// route the input
	DoMove(MovementVector.X, MovementVector.Y);
}

void Au560fpsCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// route the input
	DoLook(LookAxisVector.X, LookAxisVector.Y);
}

void Au560fpsCharacter::DoMove(float Right, float Forward)
{
	if (GetController() != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = GetController()->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement
		AddMovementInput(ForwardDirection, Forward);
		AddMovementInput(RightDirection, Right);
	}
}

void Au560fpsCharacter::DoLook(float Yaw, float Pitch)
{
	if (GetController() != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void Au560fpsCharacter::DoJumpStart()
{
	// signal the character to jump
	Jump();
}

void Au560fpsCharacter::DoJumpEnd()
{
	// signal the character to stop jumping
	StopJumping();
}
