#include "h8687/m8687.h"
QVector<double> m8687::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
