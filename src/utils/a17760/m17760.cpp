#include "a17760/m17760.h"
QVector<double> m17760::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
