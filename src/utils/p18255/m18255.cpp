#include "p18255/m18255.h"
QVector<double> m18255::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
