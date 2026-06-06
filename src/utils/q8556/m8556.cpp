#include "q8556/m8556.h"
QVector<double> m8556::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
