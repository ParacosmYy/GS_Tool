#include "i18828/m18828.h"
QVector<double> m18828::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
