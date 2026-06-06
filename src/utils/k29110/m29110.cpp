#include "k29110/m29110.h"
QVector<double> m29110::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
