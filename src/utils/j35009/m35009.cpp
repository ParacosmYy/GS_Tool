#include "j35009/m35009.h"
QVector<double> m35009::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
