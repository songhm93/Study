// Copyright Epic Games, Inc. All Rights Reserved.
/*===========================================================================
	Generated code exported from UnrealHeaderTool.
	DO NOT modify this manually! Edit the corresponding .h files instead!
===========================================================================*/

#include "UObject/ObjectMacros.h"
#include "UObject/ScriptMacros.h"

PRAGMA_DISABLE_DEPRECATION_WARNINGS
#ifdef SHOOTER_ShooterAnimInstance_generated_h
#error "ShooterAnimInstance.generated.h already included, missing '#pragma once' in ShooterAnimInstance.h"
#endif
#define SHOOTER_ShooterAnimInstance_generated_h

#define FID_Shooter_Source_Shooter_ShooterAnimInstance_h_13_SPARSE_DATA
#define FID_Shooter_Source_Shooter_ShooterAnimInstance_h_13_RPC_WRAPPERS \
 \
	DECLARE_FUNCTION(execUpdateAnimationProperties);


#define FID_Shooter_Source_Shooter_ShooterAnimInstance_h_13_RPC_WRAPPERS_NO_PURE_DECLS \
 \
	DECLARE_FUNCTION(execUpdateAnimationProperties);


#define FID_Shooter_Source_Shooter_ShooterAnimInstance_h_13_INCLASS_NO_PURE_DECLS \
private: \
	static void StaticRegisterNativesUShooterAnimInstance(); \
	friend struct Z_Construct_UClass_UShooterAnimInstance_Statics; \
public: \
	DECLARE_CLASS(UShooterAnimInstance, UAnimInstance, COMPILED_IN_FLAGS(0 | CLASS_Transient), CASTCLASS_None, TEXT("/Script/Shooter"), NO_API) \
	DECLARE_SERIALIZER(UShooterAnimInstance)


#define FID_Shooter_Source_Shooter_ShooterAnimInstance_h_13_INCLASS \
private: \
	static void StaticRegisterNativesUShooterAnimInstance(); \
	friend struct Z_Construct_UClass_UShooterAnimInstance_Statics; \
public: \
	DECLARE_CLASS(UShooterAnimInstance, UAnimInstance, COMPILED_IN_FLAGS(0 | CLASS_Transient), CASTCLASS_None, TEXT("/Script/Shooter"), NO_API) \
	DECLARE_SERIALIZER(UShooterAnimInstance)


#define FID_Shooter_Source_Shooter_ShooterAnimInstance_h_13_STANDARD_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UShooterAnimInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UShooterAnimInstance) \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UShooterAnimInstance); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UShooterAnimInstance); \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UShooterAnimInstance(UShooterAnimInstance&&); \
	NO_API UShooterAnimInstance(const UShooterAnimInstance&); \
public:


#define FID_Shooter_Source_Shooter_ShooterAnimInstance_h_13_ENHANCED_CONSTRUCTORS \
	/** Standard constructor, called after all reflected properties have been initialized */ \
	NO_API UShooterAnimInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get()) : Super(ObjectInitializer) { }; \
private: \
	/** Private move- and copy-constructors, should never be used */ \
	NO_API UShooterAnimInstance(UShooterAnimInstance&&); \
	NO_API UShooterAnimInstance(const UShooterAnimInstance&); \
public: \
	DECLARE_VTABLE_PTR_HELPER_CTOR(NO_API, UShooterAnimInstance); \
	DEFINE_VTABLE_PTR_HELPER_CTOR_CALLER(UShooterAnimInstance); \
	DEFINE_DEFAULT_OBJECT_INITIALIZER_CONSTRUCTOR_CALL(UShooterAnimInstance)


#define FID_Shooter_Source_Shooter_ShooterAnimInstance_h_10_PROLOG
#define FID_Shooter_Source_Shooter_ShooterAnimInstance_h_13_GENERATED_BODY_LEGACY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Shooter_Source_Shooter_ShooterAnimInstance_h_13_SPARSE_DATA \
	FID_Shooter_Source_Shooter_ShooterAnimInstance_h_13_RPC_WRAPPERS \
	FID_Shooter_Source_Shooter_ShooterAnimInstance_h_13_INCLASS \
	FID_Shooter_Source_Shooter_ShooterAnimInstance_h_13_STANDARD_CONSTRUCTORS \
public: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


#define FID_Shooter_Source_Shooter_ShooterAnimInstance_h_13_GENERATED_BODY \
PRAGMA_DISABLE_DEPRECATION_WARNINGS \
public: \
	FID_Shooter_Source_Shooter_ShooterAnimInstance_h_13_SPARSE_DATA \
	FID_Shooter_Source_Shooter_ShooterAnimInstance_h_13_RPC_WRAPPERS_NO_PURE_DECLS \
	FID_Shooter_Source_Shooter_ShooterAnimInstance_h_13_INCLASS_NO_PURE_DECLS \
	FID_Shooter_Source_Shooter_ShooterAnimInstance_h_13_ENHANCED_CONSTRUCTORS \
private: \
PRAGMA_ENABLE_DEPRECATION_WARNINGS


template<> SHOOTER_API UClass* StaticClass<class UShooterAnimInstance>();

#undef CURRENT_FILE_ID
#define CURRENT_FILE_ID FID_Shooter_Source_Shooter_ShooterAnimInstance_h


PRAGMA_ENABLE_DEPRECATION_WARNINGS
