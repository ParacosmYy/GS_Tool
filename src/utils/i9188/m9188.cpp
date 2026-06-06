#include "i9188/m9188.h"
QVector<double> m9188::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
