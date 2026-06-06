#include "p16255/m16255.h"
QVector<double> m16255::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
