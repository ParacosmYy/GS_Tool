#include "i25048/m25048.h"
QVector<double> m25048::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
