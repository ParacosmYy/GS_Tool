#include "q8316/m8316.h"
QVector<double> m8316::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
