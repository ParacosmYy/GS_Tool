#include "c9522/m9522.h"
QVector<double> m9522::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
