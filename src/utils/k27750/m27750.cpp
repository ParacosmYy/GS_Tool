#include "k27750/m27750.h"
QVector<double> m27750::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
