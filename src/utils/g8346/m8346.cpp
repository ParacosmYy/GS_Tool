#include "g8346/m8346.h"
QVector<double> m8346::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
