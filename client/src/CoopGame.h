#pragma once
class MainMenu;
class Player;
struct Enemy;
namespace coop {
bool Active();
bool Networked();
bool Spawning();
unsigned Tick();
void Prepare();
void StartNetwork();
class NativeScope {
    bool engaged_;
public:
    NativeScope();
    ~NativeScope();
};
int AutoStart(MainMenu *menu);
void Stop();
void OpenSetup(MainMenu *menu);
int UpdateSetup(MainMenu *menu);
void DrawSetup();
void Update();
void Draw();
bool DamageBoss(Enemy *enemy, int damage);
bool BossProjection(const Enemy *enemy);
bool AllowDeath(Player *player);
void AfterDeath(Player *player, int previousState);
bool Support(int side, int level);
}
