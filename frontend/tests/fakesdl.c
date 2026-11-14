/* Stand-in for SDL's present calls, loaded after the vsync library in test_vsync. */
int fake_presents;
void SDL_RenderPresent(void *r) { (void)r; fake_presents++; }
int SDL_Flip(void *s) { (void)s; fake_presents++; return 0; }
