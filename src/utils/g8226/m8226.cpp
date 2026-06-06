#include "g8226/m8226.h"
QVector<double> m8226::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
