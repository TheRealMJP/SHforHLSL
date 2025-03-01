#pragma once

struct AppSettings_CBLayout
{
    uint Dummy;
};

ConstantBuffer<AppSettings_CBLayout> AppSettingsCB : register(b12);

struct AppSettings_Values
{
    uint Dummy;
};

static const AppSettings_Values AppSettings =
{
    0
};

