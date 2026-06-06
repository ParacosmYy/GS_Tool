#include "a8440/m8440.h"
QVector<double> m8440::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
