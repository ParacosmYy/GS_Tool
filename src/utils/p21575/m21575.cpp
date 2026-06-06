#include "p21575/m21575.h"
QVector<double> m21575::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
