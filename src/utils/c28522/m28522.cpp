#include "c28522/m28522.h"
QVector<double> m28522::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
