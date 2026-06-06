#include "i29488/m29488.h"
QVector<double> m29488::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
