#include "j25709/m25709.h"
QVector<double> m25709::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
