#include "m37592/m37592.h"
QVector<double> m37592::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
