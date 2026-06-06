#include "i35048/m35048.h"
QVector<double> m35048::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
