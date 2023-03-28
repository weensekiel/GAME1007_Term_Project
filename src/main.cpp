/*
* Name: Ezekiel Evangelista, Cameron Lochray
* Date: March 19, 2023
*/

// Core Libraries
#include <crtdbg.h>
#include <iostream>
#include <Windows.h>
#include <SDL_Image.h>
#include <SDL.h> //allows us to use SDL library
#include <vector> //to make a container for the projectiles


/*
* Use SDL to open a window and render some sprites at given locations and scales
*/

//global variables
constexpr float FPS = 60.0f;
constexpr float DELAY_TIME = 1000.0f / FPS; //target deltaTime in ms
const int SCREEN_WIDTH = 1200;
const int SCREEN_HEIGHT = 600; 
float deltaTime = 1 / FPS; //time passing between frames in seconds

SDL_Window* pWindow = nullptr; //pointer to SDL_Window. It stores a menory location which we can use later.
SDL_Renderer* pRenderer = nullptr;
bool isGameRunning = true;

float enemySpawnDelay = 1.0f;
float enemySpawnTimer = 0.0f;

namespace Fund
{
	//EZEKIEL
	//so getPosition() can return 2 values
	struct Vec2
	{
		float x = 0;
		float y = 0;
	};


	struct Sprite
	{
	private:
		SDL_Texture* pTexture;
		SDL_Rect src;
		SDL_Rect dst;

	public:
		double rotation = 0; //in degrees
		SDL_RendererFlip flipState = SDL_FLIP_NONE;
		Vec2 position; //where sprite is on screen

		//This is a constructor. this is a special type of function used when creating an object
		//The compiler knows it's a constructor because it has parentheses like a function, has the SAME NAME as the struct or class, and has no return. This one has no arguments. In that case, it's called the default constructor and is used to set default values.
		Sprite()
		{
			std::cout << "Sprite default constructor\n";
			pTexture = nullptr;
			src = SDL_Rect{ 0,0,0,0 };
			dst = SDL_Rect{ 0,0,0,0 };
		}

		//constructors can have arguments as well, which is handy when we need to make them different
		Sprite(SDL_Renderer* renderer, const char* filePathToLoad)
		{
			std::cout << "Sprite filepath constructor\n";
			src = SDL_Rect{ 0,0,0,0 };
			
			pTexture = IMG_LoadTexture(renderer, filePathToLoad); //load into our pTexture pointer
			if (pTexture == NULL)
			{
				std::cout << "Image failed to load: " << SDL_GetError() << std::endl;
			}
			SDL_QueryTexture(pTexture, NULL, NULL, &src.w, &src.h); //ask for the dimensions of the texture
			dst = SDL_Rect{ 0,0,src.w,src.h };
			//at this point, the width and the height of the texture should be placed at the memory addresses of src.w and src.h
		}

		//getters and setters
		
		//sets size for width and height
		void setSize(Vec2 sizeWidthHeight)
		{
			dst.w = sizeWidthHeight.x;
			dst.h = sizeWidthHeight.y;
		}

		//sets width and height
		void setSize(int w, int h)
		{
			dst.w = w;
			dst.h = h;
		}

		//return width and height
		Vec2 getSize()
		{
			Vec2 returnVec = {dst.w,dst.h};
			return returnVec;
		}

		//return x and y position
		Vec2 getPosition()
		{
			Vec2 returnVec = {dst.x,dst.y};
			return returnVec;
		}


		//this draw function can be called on individual varuables of type Fund::Sprite, which will use their own variables to call SDL_RenderCopy. So, we can declare and draw new sprites with two lines:
		//Fund::Sprite myNewSprite = Sprite(pRenderer, "somefile.png");
		//myNewSprite.Draw(pRenderer);
		void Draw(SDL_Renderer* renderer)
		{
			dst.x = position.x;
			dst.y = position.y;
			SDL_RenderCopyEx(renderer, pTexture, &src, &dst, rotation, NULL, flipState);
		}

		
	};

	class Blaster
	{
	public:
		Sprite sprite;
		Vec2 velocity; //in px per second

		//move projectiles
		void update()
		{
			sprite.position.x += velocity.x * deltaTime;
			sprite.position.y += velocity.y * deltaTime;
		}

	};

	class Ship
	{
		//datamembers
	private:
		float fireRepeatTimer = 0.0f;

	public:
		Sprite sprite;
		float moveSpeedPx = 300;
		float fireRepeatDelay = 1.0f;

		//functions
		void move(Vec2 input)
		{
			sprite.position.y += input.y * (-moveSpeedPx * deltaTime);
			sprite.position.x += input.x * (-moveSpeedPx * deltaTime);

		}

		void shoot(bool shootUpwards, std::vector<Fund::Blaster>& container, Fund::Vec2 velocity)
		{
			//create a new bullet 
			Fund::Sprite blasterSprite = Fund::Sprite(pRenderer, "../Assets/textures/blasterbolt.png");

			//start blaster at player position
			blasterSprite.position.x = sprite.getPosition().x;
			blasterSprite.position.y = sprite.getPosition().y;

			if (shootUpwards) //position of player projectiles
			{
				blasterSprite.position.x = sprite.getPosition().x + 20;
			}
			else //position of enemy projectiles
			{
				blasterSprite.position.x = sprite.getPosition().x + 14;
			}

			blasterSprite.position.y = sprite.position.y + (sprite.getSize().x / 2);

			Blaster blaster;
			blaster.sprite = blasterSprite;
			blaster.velocity = velocity;

			//add blaster to container
			container.push_back(blaster);

			//reset cooldown
			fireRepeatTimer = fireRepeatDelay;
		}

		void update()
		{
			fireRepeatTimer -= deltaTime;
		}

		bool canShoot()
		{
			return (fireRepeatTimer <= 0.0f);
		}
	};

}

//create new instances of struct Fund to load textures
Fund::Ship player;
Fund::Sprite background;
Fund::Sprite background2;
Fund::Sprite planet;
Fund::Sprite asteroid;

std::vector<Fund::Ship> enemyContainer; //container for enemy ships
std::vector<Fund::Blaster> enemyBlasterContainer; //container for enemies projectiles
std::vector<Fund::Blaster> playerBlasterContainer; //used so there can be multiple blaster projectiles on screen 

//Initialize SDL, open the window and set up renderer
bool Init()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		std::cout << "SDL Init failed: " << SDL_GetError();
		return false;
	}
	std::cout << "SDL Init success\n";

	//Create and assign our SDL_Window pointer
	pWindow = SDL_CreateWindow("Milestone 2", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
	
	//not necessary to work but help figure out why it doens't work
	if (pWindow == NULL)
	{
		std::cout << "Window creation failed: " << SDL_GetError();
		return false;
	}
	else
	{
		std::cout << "Window creation success\n";
	}

	//Create and assign out SDL_Renderer pointer
	pRenderer = SDL_CreateRenderer(pWindow, -1, 0);

	if (pRenderer == NULL) //If CreateRenderer failed...
	{
		std::cout << "Renderer creation failed: " << SDL_GetError();
		return false;
	}
	else
	{
		std::cout << "Renderer creation success\n";
	}

	return true;
}

void Load()
{
	//player textures
	char* fileToLoad = "../Assets/textures/fighter.png";

	player.sprite = Fund::Sprite(pRenderer, fileToLoad);

	Fund::Vec2 shipSize = player.sprite.getSize();
	int shipWidth = shipSize.x;
	int shipHeight = shipSize.y;

	//Describe location to paste to on the screen
	player.sprite.setSize(shipWidth, shipHeight);
	player.sprite.position = { (SCREEN_WIDTH / 2) - 50, 500 };

	//background texture
	background = Fund::Sprite(pRenderer, "../Assets/textures/stars.png");
	background.setSize(SCREEN_WIDTH, SCREEN_HEIGHT);

	background2 = Fund::Sprite(pRenderer, "../Assets/textures/stars.png");
	background2.setSize(SCREEN_WIDTH, SCREEN_HEIGHT);
	background2.position = { SCREEN_WIDTH, SCREEN_HEIGHT - (background2.getSize().y)};

	//planet texture
	planet = Fund::Sprite(pRenderer, "../Assets/textures/ring-planet.png");
	planet.position = { 300, 400 };

	//asteroid textures
	asteroid = Fund::Sprite(pRenderer, "../Assets/textures/asteroid1.png");
	asteroid.setSize(75, 75);
	asteroid.position = { 400, 350 };
}

//input variables
bool isUpPressed = false;
bool isDownPressed = false;
bool isLeftPressed = false;
bool isRightPressed = false;
bool isShootPressed = false;

void Input()
{
	//CAMERON
	SDL_Event event; //event data polled each time
	while (SDL_PollEvent(&event)) //polled until all events are handeled
	{
		//decide what to do with this event
		switch (event.type)
		{
		case(SDL_KEYDOWN):
		{
			SDL_Scancode key = event.key.keysym.scancode;
			switch (key)
			{
			case(SDL_SCANCODE_W):
			{
				isUpPressed = true;
				break;
			}

			case(SDL_SCANCODE_S):
			{
				isDownPressed = true;
				break;
			}

			case(SDL_SCANCODE_A):
			{
				isLeftPressed = true;
				break;
			}

			case(SDL_SCANCODE_D):
			{
				isRightPressed = true;
				break;
			}

			case(SDL_SCANCODE_SPACE):
			{
				isShootPressed = true;
				break;
			}
			break;
			}

			break;
		}
		case(SDL_KEYUP):
		{
			SDL_Scancode key = event.key.keysym.scancode;
			switch (key)
			{
			case(SDL_SCANCODE_W):
			{
				isUpPressed = false;
				break;
			}

			case(SDL_SCANCODE_S):
			{
				isDownPressed = false;
				break;
			}

			case(SDL_SCANCODE_A):
			{
				isLeftPressed = false;
				break;
			}

			case(SDL_SCANCODE_D):
			{
				isRightPressed = false;
				break;
			}

			case(SDL_SCANCODE_SPACE):
			{
				isShootPressed = false;
				break;
			}
			}
			break;
		}

		}
	}
}

void SpawnEnemy()
{
	enemySpawnTimer = enemySpawnDelay;

	//enemy textures
	//loading info into Sprite
	Fund::Sprite enemyShip;
	enemyShip = Fund::Sprite(pRenderer, "../Assets/textures/d7_small.png");

	//spawn enemy ships at random locations along x axis
	enemyShip.position = { ((float)(rand() % SCREEN_WIDTH) - 100), 0 };
	enemyShip.flipState = SDL_FLIP_HORIZONTAL;
	enemyShip.rotation = 270.0;

	//loading Sprite into Ship
	Fund::Ship enemy1;
	enemy1.sprite = enemyShip;


	enemy1.fireRepeatDelay = 2.0f;
	enemy1.moveSpeedPx = 100;

	enemyContainer.push_back(enemy1);
	//reset spawn timer
	enemySpawnTimer = enemySpawnDelay;
}

void UpdatePlayer()
{
	Fund::Vec2 inputVector;

	if (isUpPressed)
	{
		inputVector.y = 1;

		//prevents player sprite from leaving the screen
		if (player.sprite.position.y < 0)
		{
			player.sprite.position.y = 0;
		}
	}

	if (isDownPressed)
	{
		inputVector.y = -1;

		//prevents player sprite from leaving the screen
		if (player.sprite.position.y > SCREEN_HEIGHT - player.sprite.getSize().y)
		{
			player.sprite.position.y = SCREEN_HEIGHT - player.sprite.getSize().y;
		}
	}

	if (isLeftPressed)
	{
		inputVector.x = 1;

		//prevents player sprite from leaving the screen
		if (player.sprite.position.x < 0)
		{
			player.sprite.position.x = 0;
		}
	}

	if (isRightPressed)
	{
		inputVector.x = -1;

		//prevents player sprite from leaving the screen
		if (player.sprite.position.x > SCREEN_WIDTH - player.sprite.getSize().x)
		{
			player.sprite.position.x = SCREEN_WIDTH - player.sprite.getSize().x;
		}
	}

	//if shooting and our shooting is off cooldown
	if (isShootPressed && player.canShoot())
	{
		bool shootUpwards = true;
		Fund::Vec2 velocity = { 0, -1000 }; //speed of player projectiles

		player.shoot(shootUpwards, playerBlasterContainer, velocity);
	}

	player.move(inputVector);
	player.update();
}

	
void Update()
{
	UpdatePlayer();

	//move all player blasters across the screen
	for (int i = 0; i < playerBlasterContainer.size(); i++)
	{
		playerBlasterContainer[i].update();
	}

	//move enemy projectiles 
	for (int i = 0; i < enemyBlasterContainer.size(); i++)
	{
		enemyBlasterContainer[i].update();
	}

	//update enemies
	for (int i = 0; i < enemyContainer.size(); i++)
	{
		//gets reference to enemy in enemyContainer[]
		Fund::Ship& enemy = enemyContainer[i];

		enemy.move({ 0, -1 });


		enemy.update();
		if (enemy.canShoot()) //allows enemies to shoot back
		{	
			bool shootUpwards = false;
			Fund::Vec2 velocity = { 0, 500 }; //speed of enemy projectiles
			enemy.shoot(shootUpwards, enemyBlasterContainer, velocity);
		}
	}

	if (enemySpawnTimer <= 0)
	{
		SpawnEnemy();
	}
	else
	{
		enemySpawnTimer -= deltaTime;
	}

	planet.position.y += 1;
	if (planet.position.y >= SCREEN_HEIGHT)
	{
		planet.position.y = -200;
	}

	//STILL WORKING ON THIS IQALUWDHLIQWUHDLIQWUHDLIUHALIWUSDHLUIASHDILUSAD
	background.position.y += 1;
	if (background.position.y >= SCREEN_HEIGHT)
	{
		background.position.y = -200;

	}
	//STILL WORKING ON THIS IQALUWDHLIQWUHDLIQWUHDLIUHALIWUSDHLUIASHDILUSAD



}

void Draw()
{	
	//changes background color
	SDL_SetRenderDrawColor(pRenderer, 0, 0, 0, 0);

	// refreshes the frame so ship doesn't smear when it moves
	SDL_RenderClear(pRenderer);
	background.Draw(pRenderer);
	planet.Draw(pRenderer);
	asteroid.Draw(pRenderer);
	player.sprite.Draw(pRenderer);

	//draw all player blasters on the screen
	for (int i = 0; i < playerBlasterContainer.size(); i++)
	{
		playerBlasterContainer[i].sprite.Draw(pRenderer);
	}

	//draw all enemy blasters on the screen
	for (int i = 0; i < enemyBlasterContainer.size(); i++)
	{
		enemyBlasterContainer[i].sprite.Draw(pRenderer);
	}

	//draw all enemy ships on the screen
	for (int i = 0; i < enemyContainer.size(); i++)
	{
		enemyContainer[i].sprite.Draw(pRenderer);
	}

	//show the hidden space we were drawing to called the BackBuffer. 
	//For more information why we use this, look up Double Buffering
	SDL_RenderPresent(pRenderer);
}

/**
 * \brief Program Entry Point
 */
int main(int argc, char* args[])
{

	// show and position the application console
	AllocConsole();
	auto console = freopen("CON", "w", stdout);
	const auto window_handle = GetConsoleWindow();
	MoveWindow(window_handle, 100, 700, 800, 200, TRUE);

	// Display Main SDL Window
	isGameRunning = Init();

	Load();

	// Main Game Loop
	while (isGameRunning)
	{
		const auto frame_start = static_cast<float>(SDL_GetTicks());

		Input();//take player input

		Update();//update game state (presumably based on other conditions and input)

		Draw();//draw to screen to show new game state to player

		//figure out how long we need to wait for the next frame timing
		//current time - time at start of frame = time elapsed during this frame

		if (const float frame_time = static_cast<float>(SDL_GetTicks()) - frame_start;
			frame_time < DELAY_TIME)
		{
			SDL_Delay(static_cast<int>(DELAY_TIME - frame_time));
		}

		// delta time
		deltaTime = (static_cast<float>(SDL_GetTicks()) - frame_start) / 1000.0f;

	}



	return 0;
}

