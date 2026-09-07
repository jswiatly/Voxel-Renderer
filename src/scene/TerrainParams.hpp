#pragma once

struct TerrainParams {
    int seed = 1337;
    int worldSize = 512;

    float continentLo = 0.30f;
    float continenHi = 0.48f;
    float oceanFloor = -14.0f;
    float oceanRelief = 8.0f;

    float alpineStart = 14.0f;
    float alpineEnd = 28.0f;
    float desertThreshold = 0.74f;
    int soilDepth = 5;
    int cliffSlope = 3;
    int beachRadius = 6;
    int beachHeight = 3;

    int treeCell = 10;
    float forestLo = 0.45f;
    float forestHi = 0.70f;
    int trunkMin = 4;
    int trunkVar = 3;

    float plainsFreq = 0.01f;
    float mountainFreq = 0.03f;
    float continentFreq = 0.004f;
    float warpFreq = 0.01f;
    float warpStrength = 80.0f;
};