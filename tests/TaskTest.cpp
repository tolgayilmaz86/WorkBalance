#include <gtest/gtest.h>
#include "core/Task.h"

using namespace WorkBalance::Core;

// Task struct tests
class TaskTest : public ::testing::Test {
  protected:
    Task task{"Test Task", false, 4, 2};
};

TEST_F(TaskTest, IsCompleteWhenMarkedComplete) {
    task.completed = true;
    EXPECT_TRUE(task.isComplete());
}

TEST_F(TaskTest, IsNotCompleteWhenNotMarked) {
    task.completed = false;
    EXPECT_FALSE(task.isComplete());
}

TEST_F(TaskTest, ProgressCalculation) {
    // 2 of 4 pomodoros = 50%
    EXPECT_FLOAT_EQ(task.getProgress(), 0.5f);
}

TEST_F(TaskTest, ProgressAtZero) {
    task.completed_pomodoros = 0;
    task.estimated_pomodoros = 4;
    EXPECT_FLOAT_EQ(task.getProgress(), 0.0f);
}

TEST_F(TaskTest, ProgressAt100Percent) {
    task.completed_pomodoros = 4;
    task.estimated_pomodoros = 4;
    EXPECT_FLOAT_EQ(task.getProgress(), 1.0f);
}

TEST_F(TaskTest, ProgressOver100Percent) {
    task.completed_pomodoros = 5;
    task.estimated_pomodoros = 4;
    EXPECT_GT(task.getProgress(), 1.0f);
}

TEST_F(TaskTest, ProgressWithZeroEstimate) {
    task.estimated_pomodoros = 0;
    task.completed_pomodoros = 0;
    // Should handle division by zero gracefully
    float progress = task.getProgress();
    // Either 0 or some defined behavior (not NaN or crash)
    EXPECT_FALSE(std::isnan(progress));
}

// TaskManager tests
class TaskManagerTest : public ::testing::Test {
  protected:
    TaskManager manager;
};

TEST_F(TaskManagerTest, InitiallyEmpty) {
    EXPECT_EQ(manager.getTaskCount(), 0u);
    EXPECT_TRUE(manager.getTasks().empty());
}

TEST_F(TaskManagerTest, AddTask) {
    manager.addTask("New Task", 3);

    EXPECT_EQ(manager.getTaskCount(), 1u);
    EXPECT_EQ(manager.getTask(0)->name, "New Task");
    EXPECT_EQ(manager.getTask(0)->estimated_pomodoros, 3);
    EXPECT_EQ(manager.getTask(0)->completed_pomodoros, 0);
    EXPECT_FALSE(manager.getTask(0)->completed);
}

TEST_F(TaskManagerTest, AddTaskWithDefaultEstimate) {
    manager.addTask("Default Task");

    EXPECT_EQ(manager.getTask(0)->estimated_pomodoros, 1);
}

TEST_F(TaskManagerTest, RemoveTask) {
    manager.addTask("Task 1");
    manager.addTask("Task 2");
    manager.addTask("Task 3");

    manager.removeTask(1); // Remove "Task 2"

    EXPECT_EQ(manager.getTaskCount(), 2u);
    EXPECT_EQ(manager.getTask(0)->name, "Task 1");
    EXPECT_EQ(manager.getTask(1)->name, "Task 3");
}

TEST_F(TaskManagerTest, RemoveTaskOutOfBounds) {
    manager.addTask("Only Task");
    // Should not crash
    manager.removeTask(100);
    EXPECT_EQ(manager.getTaskCount(), 1u);
}

TEST_F(TaskManagerTest, UpdateTask) {
    manager.addTask("Original", 2);

    manager.updateTask(0, "Updated", 4, 1);

    const Task* task = manager.getTask(0);
    EXPECT_EQ(task->name, "Updated");
    EXPECT_EQ(task->estimated_pomodoros, 4);
    EXPECT_EQ(task->completed_pomodoros, 1);
}

TEST_F(TaskManagerTest, ToggleTaskCompletion) {
    manager.addTask("Task", 1);

    EXPECT_FALSE(manager.getTask(0)->completed);

    manager.toggleTaskCompletion(0);
    EXPECT_TRUE(manager.getTask(0)->completed);

    manager.toggleTaskCompletion(0);
    EXPECT_FALSE(manager.getTask(0)->completed);
}

TEST_F(TaskManagerTest, IncrementTaskPomodoros) {
    manager.addTask("Task", 4);

    EXPECT_EQ(manager.getTask(0)->completed_pomodoros, 0);

    manager.incrementTaskPomodoros(0);
    EXPECT_EQ(manager.getTask(0)->completed_pomodoros, 1);

    manager.incrementTaskPomodoros(0);
    EXPECT_EQ(manager.getTask(0)->completed_pomodoros, 2);
}

TEST_F(TaskManagerTest, GetIncompleteTasks) {
    manager.addTask("Task 1", 1);
    manager.addTask("Task 2", 1);
    manager.addTask("Task 3", 1);

    manager.toggleTaskCompletion(1); // Complete Task 2

    auto incomplete = manager.getIncompleteTasks();

    EXPECT_EQ(incomplete.size(), 2u);
    EXPECT_EQ(incomplete[0]->name, "Task 1");
    EXPECT_EQ(incomplete[1]->name, "Task 3");
}

TEST_F(TaskManagerTest, GetTaskReturnsNullForInvalidIndex) {
    manager.addTask("Task");

    EXPECT_EQ(manager.getTask(100), nullptr);
}

TEST_F(TaskManagerTest, GetCompletedPomodoros) {
    manager.addTask("Task 1", 3);
    manager.addTask("Task 2", 2);

    manager.incrementTaskPomodoros(0);
    manager.incrementTaskPomodoros(0);
    manager.incrementTaskPomodoros(1);

    EXPECT_EQ(manager.getCompletedPomodoros(), 3);
}

TEST_F(TaskManagerTest, GetTargetPomodoros) {
    manager.addTask("Task 1", 3);
    manager.addTask("Task 2", 2);

    EXPECT_EQ(manager.getTargetPomodoros(), 5);
}

TEST_F(TaskManagerTest, Clear) {
    manager.addTask("Task 1");
    manager.addTask("Task 2");

    manager.clear();

    EXPECT_EQ(manager.getTaskCount(), 0u);
    EXPECT_TRUE(manager.getTasks().empty());
}

TEST_F(TaskManagerTest, GetTasksSpan) {
    manager.addTask("Task 1");
    manager.addTask("Task 2");

    auto tasks = manager.getTasks();

    EXPECT_EQ(tasks.size(), 2u);
    EXPECT_EQ(tasks[0].name, "Task 1");
    EXPECT_EQ(tasks[1].name, "Task 2");
}

TEST_F(TaskManagerTest, MutableTaskAccess) {
    manager.addTask("Original");

    Task* task = manager.getTask(0);
    task->name = "Modified";

    EXPECT_EQ(manager.getTask(0)->name, "Modified");
}
