#pragma once

struct TerrainParams {
    int seed = 1337;
    int worldSize = 512;

    // Terrain height
    float baseFrequency = 0.005f;
    float baseAmplitude = 35.0f;

    float hillFrequency = 0.008f;
    float hillAmplitude = 45.0f;

    float detailFrequency = 0.05f;
    float detailAmplitude = 3.0f;

    float seaLevel = 0.0f;

    // Surface
    int soilDepth = 4;

    // Compatibility with current surface/tree generation
    int cliffSlope = 3;
    int beachRadius = 6;
    int beachHeight = 3;
    float desertThreshold = 0.74f;
    float alpineStart = 14.0f;
    float alpineEnd = 28.0f;

    // Trees
    int treeCell = 10;
    float forestLo = 0.45f;
    float forestHi = 0.70f;
    int trunkMin = 4;
    int trunkVar = 3;
};