import { Metadata } from 'next';
import Link from 'next/link';

export const metadata: Metadata = {
  title: 'RUSH CUP - CS2 2v2 Tournament',
  description: 'Join the RUSH CUP CS2 tournament. 2v2 format, 8 teams, 40 PLN prize pool. Read the rules and register your team.',
};

const regulamin = [
  {
    title: '1. Informacje ogólne',
    items: [
      'RUSH CUP jest turniejem społecznościowym w grze Counter-Strike 2.',
      'Turniej jest rozgrywany w formacie drużynowym 2v2.',
      'Planowana liczba drużyn wynosi 8.',
      'Dokładna data, godziny oraz format fazy play-off zostaną podane przez organizatorów przed rozpoczęciem turnieju.',
      'Organizator ma prawo zmienić format lub harmonogram turnieju, jeżeli wymaga to sytuacja organizacyjna.',
    ],
  },
  {
    title: '2. Uczestnicy',
    items: [
      'Drużyna składa się z 2 zawodników.',
      'Organizator może dopuścić zawodników rezerwowych.',
      'Zabronione jest podszywanie się pod innego zawodnika.',
      'Każdy uczestnik musi posiadać konto Counter-Strike 2 pozwalające na udział w rozgrywkach.',
    ],
  },
  {
    title: '3. Zapisy',
    items: [
      'Drużyna zgłasza się poprzez formularz lub kanał wskazany przez organizatora.',
      'Zgłoszenie powinno zawierać nazwę drużyny oraz dane wymagane przez organizatora.',
      'Zgłoszenie nie oznacza automatycznego przyjęcia do turnieju.',
      'Organizator potwierdza udział drużyny.',
      'Po zamknięciu zapisów zmiany w składzie wymagają zgody organizatora.',
    ],
  },
  {
    title: '4. Mecze',
    items: [
      'Mecze rozgrywane są w Counter-Strike 2.',
      'Dokładny format meczu, system wyboru map oraz sposób rozstrzygania remisów zostaną podane przed turniejem.',
      'Drużyny muszą być gotowe do rozpoczęcia meczu o wyznaczonej godzinie.',
      'Spóźnienie może skutkować walkowerem.',
      'Organizator może przesunąć godzinę rozpoczęcia meczu, jeżeli wystąpią problemy techniczne lub organizacyjne.',
    ],
  },
  {
    title: '5. Mapy i serwer',
    items: [
      'Mapy są wybierane zgodnie z formatem określonym przez organizatora.',
      'Mecze powinny być rozgrywane na serwerze wskazanym przez organizatora.',
      'Problemy techniczne należy zgłaszać organizatorowi możliwie szybko.',
      'Organizator może zarządzić powtórzenie części lub całego meczu, jeżeli wystąpi problem techniczny mający istotny wpływ na wynik.',
    ],
  },
  {
    title: '6. Oszustwa i niedozwolone działania',
    items: [
      'Zabronione jest używanie cheatów, skryptów, niedozwolonych programów oraz innych narzędzi zapewniających nieuczciwą przewagę.',
      'Zabronione jest wykorzystywanie błędów gry w celu uzyskania przewagi, jeżeli organizator uzna dane wykorzystanie za niedozwolone.',
      'Zabronione jest celowe zakłócanie meczu lub przeszkadzanie przeciwnikom.',
      'W przypadku podejrzenia oszustwa organizator może poprosić o dodatkowe informacje lub materiały potrzebne do wyjaśnienia sytuacji.',
      'Udowodnione oszustwo może skutkować dyskwalifikacją zawodnika lub całej drużyny.',
    ],
  },
  {
    title: '7. Zachowanie zawodników',
    items: [
      'Uczestnicy są zobowiązani do zachowania zgodnego z zasadami fair play.',
      'Zabronione są groźby, uporczywe obrażanie, nękanie oraz celowe prowokowanie innych uczestników.',
      'Organizator może ukarać zawodnika lub drużynę za rażące naruszenie zasad zachowania.',
      'Kary mogą obejmować ostrzeżenie, zmianę wyniku, walkower lub dyskwalifikację.',
    ],
  },
  {
    title: '8. Wyniki i protesty',
    items: [
      'Wynik meczu powinien zostać zgłoszony organizatorowi zgodnie z podaną procedurą.',
      'Protest dotyczący meczu należy zgłosić możliwie szybko po jego zakończeniu.',
      'Protest powinien zawierać opis sytuacji oraz, jeżeli jest dostępny, odpowiedni dowód.',
      'Ostateczną decyzję w sprawach spornych podejmuje organizator.',
      'Decyzje organizatora dotyczące interpretacji regulaminu są wiążące w ramach turnieju.',
    ],
  },
  {
    title: '9. Nagrody',
    items: [
      'Aktualna planowana pula nagród wynosi <strong>40 PLN</strong>.',
      'Podział nagród zostanie podany przed rozpoczęciem turnieju.',
      'Organizator może zmienić wysokość lub sposób podziału nagród, jeżeli zmieni się pula nagród.',
      'Jeżeli nagrody są finansowane przez sponsora, ich wypłata lub przekazanie może być uzależnione od warunków określonych przez sponsora.',
      'Szczegółowe informacje dotyczące odbioru nagród zostaną przekazane zwycięzcom.',
    ],
  },
  {
    title: '10. Transmisja',
    items: [
      'Wybrane mecze mogą być transmitowane na oficjalnych kanałach RUSH CUP.',
      'Organizator może wykorzystywać materiały z transmisji do promocji turnieju.',
      'Uczestnicy powinni stosować się do zasad dotyczących transmisji przekazanych przez organizatora.',
    ],
  },
  {
    title: '11. Zmiany regulaminu',
    items: [
      'Organizator może zmienić regulamin przed rozpoczęciem turnieju.',
      'O istotnych zmianach uczestnicy zostaną poinformowani.',
      'Organizator nie powinien zmieniać zasad w trakcie trwającego meczu w sposób wpływający na jego wynik, z wyjątkiem sytuacji wymagających interwencji organizacyjnej lub technicznej.',
    ],
  },
  {
    title: '12. Postanowienia końcowe',
    items: [
      'Udział w turnieju oznacza akceptację niniejszego regulaminu.',
      'Nieznajomość regulaminu nie zwalnia uczestnika z obowiązku jego przestrzegania.',
      'Organizator ma prawo podejmować decyzje w sytuacjach, których regulamin nie przewiduje, kierując się zasadą fair play.',
      'Wszelkie informacje dotyczące turnieju będą publikowane przez oficjalne kanały RUSH CUP.',
      'Szczegółowe informacje organizacyjne, w tym data, godziny, serwer, system rozgrywek i dokładny podział nagród, zostaną opublikowane przed rozpoczęciem turnieju.',
    ],
  },
];

export default function RushCupPage() {
  return (
    <div className="min-h-screen">
      <header className="border-b border-[var(--border)] sticky top-0 z-40 bg-[var(--background)]/95 backdrop-blur supports-[backdrop-filter]:bg-[var(--background)]/80">
        <div className="container">
          <div className="flex h-16 items-center justify-between">
            <Link href="/" className="font-bold text-xl tracking-tight" aria-label="CS2 Boost Home">
              CS2 Boost
            </Link>
            <nav className="flex items-center gap-6">
              <Link href="#features" className="text-sm font-medium text-[var(--muted-foreground)] hover:text-[var(--foreground)] transition-colors">
                Features
              </Link>
              <Link href="#pricing" className="text-sm font-medium text-[var(--muted-foreground)] hover:text-[var(--foreground)] transition-colors">
                Pricing
              </Link>
              <Link href="/rushcup" className="btn btn-primary text-sm">
                RUSH CUP
              </Link>
            </nav>
          </div>
        </div>
      </header>

      <main>
        <section className="relative py-20 lg:py-32 overflow-hidden">
          <div className="container">
            <div className="max-w-3xl mx-auto text-center">
              <div className="inline-flex items-center gap-2 px-3 py-1 rounded-full bg-[var(--primary)]/10 border border-[var(--primary)]/20 text-[var(--primary)] text-sm font-medium mb-6">
                <span className="relative flex h-2 w-2">
                  <span className="animate-ping absolute inline-flex h-full w-full rounded-full bg-[var(--primary)] opacity-75"></span>
                  <span className="relative inline-flex rounded-full h-2 w-2 bg-[var(--primary)]"></span>
                </span>
                Tournament Live
              </div>
              <h1 className="text-4xl lg:text-6xl font-bold tracking-tight mb-6">
                RUSH CUP
                <br />
                <span className="text-[var(--primary)]">CS2 2v2 Tournament</span>
              </h1>
              <p className="text-lg lg:text-xl text-[var(--muted-foreground)] mb-8 max-w-2xl mx-auto">
                Community tournament for Counter-Strike 2. 2v2 format, 8 teams, 40 PLN prize pool. 
                Register your team and compete for glory.
              </p>
              <div className="flex flex-col sm:flex-row items-center justify-center gap-4">
                <a href="mailto:tournament@cs2boost.com?subject=RUSH%20CUP%20Registration" className="btn btn-primary text-lg px-8 py-3">
                  Register Team
                </a>
                <Link href="#regulamin" className="btn btn-outline text-lg px-8 py-3">
                  Read Rules
                </Link>
              </div>
            </div>
          </div>
        </section>

        <section className="py-20 lg:py-28 bg-[var(--muted)]/50">
          <div className="container">
            <div className="grid grid-cols-1 md:grid-cols-3 gap-6 mb-12">
              <article className="card text-center group hover:border-[var(--primary)]/50 transition-colors">
                <div className="text-4xl font-bold text-[var(--primary)] mb-2">8</div>
                <div className="text-[var(--muted-foreground)]">Teams Max</div>
              </article>
              <article className="card text-center group hover:border-[var(--primary)]/50 transition-colors">
                <div className="text-4xl font-bold text-[var(--primary)] mb-2">2v2</div>
                <div className="text-[var(--muted-foreground)]">Format</div>
              </article>
              <article className="card text-center group hover:border-[var(--primary)]/50 transition-colors">
                <div className="text-4xl font-bold text-[var(--primary)] mb-2">40 PLN</div>
                <div className="text-[var(--muted-foreground)]">Prize Pool</div>
              </article>
            </div>

            <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
              <article className="card">
                <h3 className="text-lg font-semibold mb-4">Tournament Info</h3>
                <dl className="space-y-3">
                  <div className="flex justify-between">
                    <dt className="text-[var(--muted-foreground)]">Status</dt>
                    <dd className="font-medium">Upcoming</dd>
                  </div>
                  <div className="flex justify-between">
                    <dt className="text-[var(--muted-foreground)]">Format</dt>
                    <dd className="font-medium">2v2</dd>
                  </div>
                  <div className="flex justify-between">
                    <dt className="text-[var(--muted-foreground)]">Max Teams</dt>
                    <dd className="font-medium">8</dd>
                  </div>
                  <div className="flex justify-between">
                    <dt className="text-[var(--muted-foreground)]">Prize Pool</dt>
                    <dd className="font-medium">40 PLN</dd>
                  </div>
                  <div className="flex justify-between">
                    <dt className="text-[var(--muted-foreground)]">Registration</dt>
                    <dd className="font-medium">Open</dd>
                  </div>
                </dl>
              </article>
              <article className="card">
                <h3 className="text-lg font-semibold mb-4">Quick Links</h3>
                <ul className="space-y-3">
                  <li>
                    <a href="mailto:tournament@cs2boost.com?subject=RUSH%20CUP%20Registration" className="flex items-center gap-2 text-[var(--primary)] hover:underline">
                      <svg className="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24" strokeWidth="2">
                        <path strokeLinecap="round" strokeLinejoin="round" d="M18 9v3m0 0v3m0-3h3m-3 0h-3m-2-5a4 4 0 11-8 0 4 4 0 018 0zM3 20a6 6 0 0112 0v1H3v-1z" />
                      </svg>
                      Register Your Team
                    </a>
                  </li>
                  <li>
                    <a href="#regulamin" className="flex items-center gap-2 text-[var(--primary)] hover:underline">
                      <svg className="w-5 h-5" fill="none" stroke="currentColor" viewBox="0 0 24 24" strokeWidth="2">
                        <path strokeLinecap="round" strokeLinejoin="round" d="M9 12h6m-6 4h6m2 5H7a2 2 0 01-2-2V5a2 2 0 012-2h5.586a1 1 0 01.707.293l5.414 5.414a1 1 0 01.293.707V19a2 2 0 01-2 2z" />
                      </svg>
                      Read Full Rules
                    </a>
                  </li>
                </ul>
              </article>
            </div>
          </div>
        </section>

        <section id="regulamin" className="py-20 lg:py-28">
          <div className="container max-w-4xl">
            <header className="mb-12 text-center">
              <h2 className="text-3xl lg:text-4xl font-bold mb-4">Regulamin Turnieju</h2>
              <p className="text-[var(--muted-foreground)] max-w-2xl mx-auto">
                Please read the complete rules before registering your team. Participation implies acceptance of all terms.
              </p>
            </header>

            <article className="prose prose-invert max-w-none space-y-10">
              {regulamin.map((section, index) => (
                <section key={index} className="border-l-2 border-[var(--primary)]/30 pl-6">
                  <h3 className="text-xl font-bold mb-4">{section.title}</h3>
                  <ol className="list-decimal list-inside space-y-2 text-[var(--muted-foreground)]">
                    {section.items.map((item, itemIndex) => (
                      <li key={itemIndex} className="leading-relaxed">{item}</li>
                    ))}
                  </ol>
                </section>
              ))}
            </article>
          </div>
        </section>
      </main>

      <footer className="border-t border-[var(--border)] py-12 bg-[var(--muted)]/30">
        <div className="container">
          <div className="grid grid-cols-1 md:grid-cols-4 gap-8">
            <div>
              <h3 className="font-bold mb-4">RUSH CUP</h3>
              <p className="text-[var(--muted-foreground)] text-sm">
                Community Counter-Strike 2 tournament. 2v2 format, fair play, good vibes.
              </p>
            </div>
            <div>
              <h4 className="font-semibold mb-4">Tournament</h4>
              <ul className="space-y-2 text-sm text-[var(--muted-foreground)]">
                <li><a href="#regulamin" className="hover:text-[var(--foreground)]">Rules & Regulations</a></li>
                <li><a href="mailto:tournament@cs2boost.com?subject=RUSH%20CUP%20Registration" className="hover:text-[var(--foreground)]">Team Registration</a></li>
                <li><a href="#" className="hover:text-[var(--foreground)]">Bracket & Schedule</a></li>
                <li><a href="#" className="hover:text-[var(--foreground)]">Results & Stats</a></li>
              </ul>
            </div>
            <div>
              <h4 className="font-semibold mb-4">CS2 Boost</h4>
              <ul className="space-y-2 text-sm text-[var(--muted-foreground)]">
                <li><Link href="#pricing" className="hover:text-[var(--foreground)]">Boosting Services</Link></li>
                <li><Link href="/terms" className="hover:text-[var(--foreground)]">Terms of Service</Link></li>
                <li><Link href="/privacy" className="hover:text-[var(--foreground)]">Privacy Policy</Link></li>
              </ul>
            </div>
            <div>
              <h4 className="font-semibold mb-4">Contact</h4>
              <ul className="space-y-2 text-sm text-[var(--muted-foreground)]">
                <li>Discord: <a href="#" className="hover:text-[var(--foreground)]">RUSH CUP</a></li>
                <li>Email: <a href="mailto:tournament@cs2boost.com" className="hover:text-[var(--foreground)]">tournament@cs2boost.com</a></li>
              </ul>
            </div>
          </div>
          <div className="border-t border-[var(--border)] mt-8 pt-8 text-center text-sm text-[var(--muted-foreground)]">
            <p>© {new Date().getFullYear()} RUSH CUP. All rights reserved.</p>
          </div>
        </div>
      </footer>
    </div>
  );
}