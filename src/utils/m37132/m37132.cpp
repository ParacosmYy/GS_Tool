#include "m37132/m37132.h"
QVector<double> m37132::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
