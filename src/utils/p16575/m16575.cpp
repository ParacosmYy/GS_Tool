#include "p16575/m16575.h"
QVector<double> m16575::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
