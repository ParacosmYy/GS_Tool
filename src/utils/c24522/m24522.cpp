#include "c24522/m24522.h"
QVector<double> m24522::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
