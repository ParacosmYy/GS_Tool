#include "a37060/m37060.h"
QVector<double> m37060::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
