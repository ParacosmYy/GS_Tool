#include "p16835/m16835.h"
QVector<double> m16835::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
