#include "f11025/m11025.h"
QVector<double> m11025::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
