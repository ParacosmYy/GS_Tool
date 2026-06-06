#include "a8160/m8160.h"
QVector<double> m8160::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
