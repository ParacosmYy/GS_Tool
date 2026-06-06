#include "p9575/m9575.h"
QVector<double> m9575::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
