#include "k29870/m29870.h"
QVector<double> m29870::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
