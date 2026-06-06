#include "c16522/m16522.h"
QVector<double> m16522::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
