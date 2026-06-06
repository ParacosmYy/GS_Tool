#include "i18708/m18708.h"
QVector<double> m18708::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
