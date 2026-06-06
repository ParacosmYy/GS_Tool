#include "m32252/m32252.h"
QVector<double> m32252::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
