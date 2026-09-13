#include<raylib.h>
#include<vector>
#include<iostream>
#include<algorithm>
#include<optional>
#include<print>

enum class SpielStatus {
  NichtAktiv, //hat nie gestartet
  Aktiv,    
  RundeEnde, //Sieger
};  

enum class TokenFarbe {
  Rot,
  Blau,
  Grün,
  Gelb,
  Lila
};

using TokenFarben = std::vector<TokenFarbe>;

class Spalte {
  public:
  explicit Spalte(const size_t zeilen) : überbleibend_(zeilen) {
    if(!zeilen){
      throw std::logic_error("Muss mehr als 0 Zeilen haben!");
    }
    farben_.reserve(zeilen); //Anzahl der Tokens pro Spalte
  }

  const TokenFarbe& operator[](const size_t zeilen) const {
    const auto existierendeZeilen = farben_.size();
    if(zeilen >= existierendeZeilen) {
      throw std::out_of_range("index außerhalb!");
    }
      /*weil das erste token oben anfängt rechnen wir um, und kriegen die werte von unten. Wenn der User bei 4 tokens
      Stelle 1 möchste kriegt er 4 - 1 -1 also index[3]; von oben Stelle 1*/
      const auto index = existierendeZeilen - zeilen - 1;
      return farben_[index];
    }

  [[nodiscard]] bool TokenPlatziert(const size_t zeile) const {
    return zeile < farben_.size();
  }

  [[nodiscard]] bool KannPlatzieren() const {
    return überbleibend_;
  }

    [[nodiscard]] bool IstVoll() const {
    return !KannPlatzieren();
  }

  [[nodiscard]] int Höhe() const {
    return überbleibend_ + farben_.size();
  }

  [[nodiscard]] int Größe() const { return farben_.size();}

  void Platzieren(const TokenFarbe color) {
    if(IstVoll()) {
      throw std::logic_error("Bereits Voll!");
    }
    überbleibend_ -= 1;
    farben_.emplace_back(color);
}
  void Reset() {
    überbleibend_ = farben_.size();
    farben_.clear();
  }
private:
  size_t überbleibend_;
  TokenFarben farben_;
};

using Spalten = std::vector<Spalte>;

class Brett {
  public:
  explicit Brett(const size_t seite) : Brett(seite, seite) {} //man könnte auch nicht delegieren -> brett_(seite, Spalte(seite)) {}
  explicit Brett(const size_t spalten, const size_t zeilen) : 
    brett_(spalten, Spalte(zeilen)) {}

  [[nodiscard]] bool TryPlaceToken(const size_t spalte, const TokenFarbe token) {
    if(KeineSpalten()) { return false;} //beende
    if(spalte >= brett_.size()) { throw std::out_of_range("Spalte zu großer Wert");}

    auto &zeilen = brett_.at(spalte); //Gibt das Spalte Objekt

    if(!zeilen.KannPlatzieren()) {
      return false;
    }

    zeilen.Platzieren(token);
    return true;
  }
  
  [[nodiscard]] bool KeineSpalten() const { return brett_.empty();}
/*Spielregeln*/
  [[nodiscard]] bool Unentschieden() const {
    return std::ranges::all_of(brett_, [](const Spalte& spalte) {
      return spalte.IstVoll();
    });
  }

  int PrüfeRichtungen(const int xEbene,
      const int yEbene,
      const int zeile, //i
      const int spalte,
      const TokenFarbe token) const {
        if(spalte >= brett_.size()) {
          return 0;
        }

        auto& spalte1 = brett_.at(spalte);

        if(zeile >= spalte1.Höhe() || !spalte1.TokenPlatziert(zeile)) {
          return 0;
        }

        const auto existierend = spalte1[zeile];

        return existierend == token ? //Wenn JA dann solange rekursiv +1 in eine Richtung prüfen bis !=
          PrüfeRichtungen(xEbene, yEbene, zeile + yEbene, spalte + xEbene, token) + 1 :
          0;
      }
      
    

  std::optional<TokenFarbe> WerGewonnen(const int spalte) const {
    const auto brettHöhe = brett_.at(spalte).Höhe();
    const int TokensMindestLänge{4};
    const static std::vector<std::pair<int, int>> richtungen {
    {1, 0}, {0, 1},  {1, 1},  {1, -1}
    };
    for(auto i{ 0uz}; i < brettHöhe; i++) { //(O)n; linear wachsend mit Zeilen; (O)n² bei Funktion im Durchlauf - ungefähr
      for(const auto& [xEbene, yEbene] : richtungen){
        if(!brett_.at(spalte).TokenPlatziert(i)) {
          continue;
        }
        const auto token = brett_.at(spalte)[i];
        const auto inZeile = PrüfeRichtungen(xEbene, yEbene, i, spalte, token);
        if(inZeile == TokensMindestLänge) { return token;}

      const auto umgekehrteZeile = PrüfeRichtungen(-xEbene, -yEbene, i, spalte, token); //andere Richtung
      if(umgekehrteZeile == TokensMindestLänge) {
        return token;
      }
      const auto totaleTokenGefunden = umgekehrteZeile + inZeile - 1; // -1 Damit der erste Token nicht dopppel gezählt wird  X X [X] X
      if(totaleTokenGefunden == TokensMindestLänge) {
        return token;
      }
    }
  }
    return std::nullopt;
  }

  void Reset() {
    for(auto& spalten : brett_) {
      spalten.Reset(); //innerhalb Spalte
    }
  }
private:
  Spalten brett_;
};

  class Spieler{ 
  public:
    Spieler(std::string name, TokenFarbe token) :
    name_(std::move(name)), token_(token) {}
    [[nodiscard]] std::string HoleNamen() const { return name_;}
    [[nodiscard]] TokenFarbe Holetoken() const { return token_;}
    static int HolNächstenZug() {
      int spalte{};
      std::cin >> spalte;
      return spalte;
    }
  private:
    std::string name_;
    TokenFarbe token_;
  };

  using Players = std::vector<Spieler>;

  class Spielerkollektion{
  public:
    explicit Spielerkollektion(Players players) : players_(std::move(players)) {}  

    const Spieler& HolNächstenSpieler() {
      const auto wechsel = wechsel_++ % players_.size();
      return players_.at(wechsel);
    }
  
    Spieler GetPlayer(const TokenFarbe token) const {
      const auto spielerIterator = std::ranges::find_if(players_, [token](const Spieler& spieler) {
        return spieler.Holetoken() == token;
      });
      if(spielerIterator == players_.end()) { throw std::logic_error("Token nicht genutzt son!");}

      return *spielerIterator;
    }
  private:
  int wechsel_;
  Players players_;
  };

class Spiel {
public:
  Spiel(const size_t spalten, Spielerkollektion spielerkollektion) : Spiel(spalten, spalten, std::move(spielerkollektion)) {}
  Spiel(const size_t spalten, const size_t zeilen,
       Spielerkollektion spielerkollektion) : brett_(spalten, zeilen),
       spielerkollektion_(std::move(spielerkollektion)) {} //moven per wert um r/-l values beide sinnvoll zu nutzen

  [[nodiscard]] SpielStatus HoleSpielstatus() const { return spielstatus_;}
  
  std::optional<TokenFarbe> Spielen() {
    spielstatus_ = SpielStatus::Aktiv;
    do{
      const auto& spieler = spielerkollektion_.HolNächstenSpieler();
      std::cout << spieler.HoleNamen() <<"'s Zug " << '\n';

      bool kannSetzen{};
      int location{};
      do{
        location = Spieler::HolNächstenZug();
        std::cout << spieler.HoleNamen() << " waehlt Spalte " << location << '\n';

        kannSetzen = brett_.TryPlaceToken(location, spieler.Holetoken()); //platziert oder nicht und gibt bool zuzrück

        if(!kannSetzen) {
            std::cout << "Kann keine Token mehr in Spalte {} setzen" << location << '\n';
        }
      } while(!kannSetzen); //immer wiederholen wenn kein token eingefügt wurde, bevor ein neuer Gewinner geprüft werden soll

      if(const auto Gewinner = brett_.WerGewonnen(location); Gewinner) {
        spielstatus_ = SpielStatus::RundeEnde;
        return Gewinner;
      }
    } while(!brett_.Unentschieden());

    spielstatus_ = SpielStatus::NichtAktiv;
    brett_.Reset();
    std::cout << "Unentschieden -> Reset!";
    return std::nullopt;
  }
private:
  Brett brett_;
  Spielerkollektion spielerkollektion_;
  SpielStatus spielstatus_{ SpielStatus::NichtAktiv};
  };
int main(){
  Spielerkollektion spielerkollektion(Players{
    Spieler{"Viertel", TokenFarbe::Blau},
    Spieler{"Andre", TokenFarbe::Gelb},
  }); //Vektor mit Spieler Objekten als Konstruktor
  Spiel spiel(10, 10, spielerkollektion);
  if(const auto ergebniss = spiel.Spielen(); ergebniss){ std::cout << spielerkollektion.GetPlayer(*ergebniss).HoleNamen() << " hat Gewonnen!";}
    else { std::cout << "Unentschieden";}
  return 0;
}