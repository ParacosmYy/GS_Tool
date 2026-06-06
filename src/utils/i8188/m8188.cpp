#include "i8188/m8188.h"
QVector<double> m8188::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
