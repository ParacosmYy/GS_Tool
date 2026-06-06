#include "p28055/m28055.h"
QVector<double> m28055::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
