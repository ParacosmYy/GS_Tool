#include "k29310/m29310.h"
QVector<double> m29310::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
