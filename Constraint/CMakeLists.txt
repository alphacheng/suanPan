target_sources(${PROJECT_NAME} PRIVATE
        Constraint/Constraint.cpp
        Constraint/ConstraintParser.cpp
        Constraint/Embed2D.cpp
        Constraint/MPC.cpp
        Constraint/ParticleCollision.cpp
        Constraint/ParticleCollision2D.cpp
        Constraint/ParticleCollision3D.cpp
        Constraint/RigidWallMultiplier.cpp
        Constraint/RigidWallPenalty.cpp
        )

add_subdirectory(BC)
add_subdirectory(Criterion)