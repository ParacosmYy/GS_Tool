#include "f9625/m9625.h"
QVector<double> m9625::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
