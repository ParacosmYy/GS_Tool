#include "p21675/m21675.h"
QVector<double> m21675::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
