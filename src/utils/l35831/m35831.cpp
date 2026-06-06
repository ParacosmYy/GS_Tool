#include "l35831/m35831.h"
QVector<double> m35831::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
