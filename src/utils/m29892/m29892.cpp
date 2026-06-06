#include "m29892/m29892.h"
QVector<double> m29892::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
