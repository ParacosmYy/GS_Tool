#include "p18035/m18035.h"
QVector<double> m18035::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
