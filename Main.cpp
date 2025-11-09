#include <iostream>
#include <array>
#include <cassert>
#include <algorithm>
#include <chrono>
#include <random>
#include <limits>

namespace Random
{
	std::mt19937 generate()
	{
		std::random_device rd{};

		std::seed_seq ss{
			static_cast<std::seed_seq::result_type>(
				std::chrono::steady_clock::now().time_since_epoch().count()
				),
			rd(), rd(), rd(),
			rd(), rd(), rd(),
			rd()
		};

		return std::mt19937{ ss };
	}

	std::mt19937 mt{ generate() };
}

struct Card
{
	enum Rank
	{
		ace, two, three, four, five,
		six, seven, eight, nine, ten,
		jack, queen, king,
		max_rank,
	};

	enum Suit
	{
		club, diamond, heart, spade,
		max_suit,
	};

	constexpr static std::array allRanks{
		ace, two, three, four, five,
		six, seven, eight, nine, ten,
		jack, queen, king
	};

	constexpr static std::array allSuits{ club, diamond, heart, spade };

	constexpr static std::array rankName{
		'A', '2', '3', '4', '5', '6', '7', '8', '9', 'T','J', 'Q', 'K'
	};

	constexpr static std::array suitName{
		'C', 'D', 'H', 'S'
	};

	static_assert(std::size(allRanks) == max_rank);
	static_assert(std::size(allSuits) == max_suit);
	static_assert(std::size(rankName) == max_rank);
	static_assert(std::size(suitName) == max_suit);

	Rank rank{};
	Suit suit{};

	friend std::ostream& operator<<(std::ostream& out, const Card& card)
	{
		out << rankName[card.rank] << suitName[card.suit];
		return out;
	}

	int value() const
	{
		static constexpr std::array values{ 11, 2, 3, 4, 5, 6, 7, 8, 9, 10, 10, 10, 10 };
		return values[rank];
	}
};

class Deck
{
private:
	std::array<Card, 52> m_cards{};
	int m_current{ -1 };
public:
	Deck()
	{
		int index{};
		for (auto suit : Card::allSuits)
			for (auto rank : Card::allRanks)
				m_cards[index++] = Card{ rank, suit };
	}

	Card dealCard()
	{
		assert(m_current + 1 < static_cast<int>(m_cards.size()));
		return m_cards[static_cast<std::size_t>(++m_current)];
	}

	void shuffle()
	{
		m_current = -1;
		std::shuffle(m_cards.begin(), m_cards.end(), Random::mt);
	}
};

struct Player
{
	int score{};
};

namespace Setting
{
	const int playerBust{ 21 };
	const int dealerStop{ 17 };
}

auto playerTurn(Player& player, Deck& deck)
{
	while (player.score < Setting::playerBust)
	{
		std::cout << "(h) to hit, or (s) to stand: ";
		char choice{};
		std::cin >> choice;

		if (!std::cin)
		{
			std::cin.clear();
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::cout << "That's invalid.\n";
			continue;
		}

		if (choice == 'h')
		{
			Card card{ deck.dealCard() };
			std::cout << "You were dealt " << Card::rankName[card.rank] << Card::suitName[card.suit]
				<< ". ";
			player.score += card.value();
			std::cout << "You now have: " << player.score << '\n';
		}
		else
			break;
	}
	return player.score <= Setting::playerBust;
}

bool play(Player& dealer, Player& player, Deck& deck)
{
	deck.shuffle();
	dealer.score += deck.dealCard().value();

	player.score += deck.dealCard().value();
	player.score += deck.dealCard().value();

	std::cout << "The dealer is showing: " << dealer.score << '\n';
	std::cout << "You have score: " << player.score << '\n';

	if (!playerTurn(player, deck))
	{
		std::cout << "You went bust!\n";
		return false;
	}

	while (dealer.score < Setting::dealerStop)
	{
		Card card{ deck.dealCard() };
		dealer.score += card.value();
		std::cout << "The dealer flips a "
			<< Card::rankName[card.rank] << Card::suitName[card.suit] << ".\t"
			<< "They now have: " << dealer.score << '\n';
	}

	if (dealer.score > Setting::playerBust)
	{
		std::cout << "The dealer went bust!\n";
		return true;
	}

	return player.score > dealer.score;
}

int main()
{
	Deck deck{};

	Player dealer{};
	Player player{};

	bool win = play(dealer, player, deck);

	if (win)
		std::cout << "You win!\n";
	else
		std::cout << "You lose!\n";

	return 0;
}