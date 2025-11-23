import pygame_ce as pygame
import random
import sys
from typing import List, Tuple

# Initialize pygame
pygame.init()

# Constants
WINDOW_SIZE = 600
GRID_SIZE = 20
GRID_COUNT = WINDOW_SIZE // GRID_SIZE
FPS = 10

# Colors
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
GREEN = (0, 255, 0)
RED = (255, 0, 0)

class Snake:
    def __init__(self):
        self.positions = [(GRID_COUNT // 2, GRID_COUNT // 2)]
        self.direction = (1, 0)
        self.length = 1
        self.score = 0

    def get_head_position(self) -> Tuple[int, int]:
        return self.positions[0]

    def update(self) -> bool:
        cur = self.get_head_position()
        x, y = self.direction
        new = ((cur[0] + x) % GRID_COUNT, (cur[1] + y) % GRID_COUNT)
        
        if new in self.positions[1:]:
            return False
            
        self.positions.insert(0, new)
        if len(self.positions) > self.length:
            self.positions.pop()
        return True

    def reset(self):
        self.positions = [(GRID_COUNT // 2, GRID_COUNT // 2)]
        self.direction = (1, 0)
        self.length = 1
        self.score = 0

    def render(self, surface: pygame.Surface):
        for p in self.positions:
            rect = pygame.Rect((p[0] * GRID_SIZE, p[1] * GRID_SIZE), (GRID_SIZE, GRID_SIZE))
            pygame.draw.rect(surface, GREEN, rect)
            pygame.draw.rect(surface, BLACK, rect, 1)

class Food:
    def __init__(self):
        self.position = (0, 0)
        self.randomize_position()

    def randomize_position(self, snake_positions: List[Tuple[int, int]] = None):
        self.position = (random.randint(0, GRID_COUNT - 1), 
                        random.randint(0, GRID_COUNT - 1))
        # Make sure food doesn't spawn on snake
        if snake_positions and self.position in snake_positions:
            self.randomize_position(snake_positions)

    def render(self, surface: pygame.Surface):
        rect = pygame.Rect((self.position[0] * GRID_SIZE, self.position[1] * GRID_SIZE), 
                          (GRID_SIZE, GRID_SIZE))
        pygame.draw.rect(surface, RED, rect)
        pygame.draw.rect(surface, BLACK, rect, 1)

def main():
    # Initialize game window
    screen = pygame.display.set_mode((WINDOW_SIZE, WINDOW_SIZE))
    pygame.display.set_caption('Snake Game')
    clock = pygame.time.Clock()
    font = pygame.font.SysFont('Arial', 25)

    # Game objects
    snake = Snake()
    food = Food()
    game_over = False

    # Game loop
    while True:
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()
            elif event.type == pygame.KEYDOWN:
                if game_over:
                    if event.key == pygame.K_r:
                        snake.reset()
                        food.randomize_position()
                        game_over = False
                else:
                    if event.key == pygame.K_UP and snake.direction != (0, 1):
                        snake.direction = (0, -1)
                    elif event.key == pygame.K_DOWN and snake.direction != (0, -1):
                        snake.direction = (0, 1)
                    elif event.key == pygame.K_LEFT and snake.direction != (1, 0):
                        snake.direction = (-1, 0)
                    elif event.key == pygame.K_RIGHT and snake.direction != (-1, 0):
                        snake.direction = (1, 0)

        if not game_over:
            # Update game state
            if not snake.update():
                game_over = True
            
            # Check if snake eats food
            if snake.get_head_position() == food.position:
                snake.length += 1
                snake.score += 10
                food.randomize_position(snake.positions)

        # Draw everything
        screen.fill(BLACK)
        snake.render(screen)
        food.render(screen)
        
        # Draw score
        score_text = font.render(f'Score: {snake.score}', True, WHITE)
        screen.blit(score_text, (10, 10))
        
        # Draw game over message
        if game_over:
            game_over_text = font.render('Game Over! Press R to restart', True, WHITE)
            text_rect = game_over_text.get_rect(center=(WINDOW_SIZE//2, WINDOW_SIZE//2))
            screen.blit(game_over_text, text_rect)
        
        pygame.display.update()
        clock.tick(FPS)

if __name__ == "__main__":
    main()
