#include "p25575/m25575.h"
QVector<double> m25575::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
