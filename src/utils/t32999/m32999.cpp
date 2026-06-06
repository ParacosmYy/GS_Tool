#include "t32999/m32999.h"
QVector<double> m32999::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
