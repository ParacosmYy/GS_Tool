#include "k29970/m29970.h"
QVector<double> m29970::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
