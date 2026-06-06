#include "k17870/m17870.h"
QVector<double> m17870::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
